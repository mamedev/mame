/***************************************************************************
 
  laser3k.c
  Driver for VTech Laser 3000 / Dick Smith Electronics "The Cat"
 
  This machine is somewhat similar to a 48K Apple II if you blur your eyes
  a lot, but it generally fits in poorly with 100% compatible machines
  (no chance of a compatible language card or auxmem) so it gets its own driver.
 
  An "emulation cartridge" is required to run Apple II software; it's unclear
  what that consists of.
 
  Banking theory:
  - 6502 has 4 banking windows, 0000-3FFF, 4000-7FFF, 8000-BFFF, C000-FFFF
  - Physical address space is 0x00000-0x3FFFF.  ROM and I/O at the top, RAM
    up to 0x2FFFF (192k max).
  - Each window has a bank number register at physical 3C07C/D/E/F
 
  Technical manual at:
  http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Computers/LASER/LASER%203000/Manuals/The%20Cat%20Technical%20Reference%20Manual.pdf
 
  TODO:
    - keyboard
    - graphics modes
    - 80-column text
    - figure out where the C800 ROM pages for the printer and FDC are (currently the 80-col f/w's pages are mapped there)
    - Centronics printer port (data at 3c090, read ack at 3c1c0, read busy at 3c1c2)
 
***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "sound/speaker.h"
#include "sound/sn76496.h"

class laser3k_state : public driver_device
{
public:
	laser3k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank0(*this, "bank0")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_speaker(*this, "speaker")
		, m_sn(*this, "sn76489")
	{ }

	required_device<m6502_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_bank0;
	required_device<address_map_bank_device> m_bank1;
	required_device<address_map_bank_device> m_bank2;
	required_device<address_map_bank_device> m_bank3;
	required_device<speaker_sound_device> m_speaker;
	required_device<sn76489_device> m_sn;

	READ8_MEMBER( ram_r );
	WRITE8_MEMBER( ram_w );
	READ8_MEMBER( io_r );
	WRITE8_MEMBER( io_w );
	READ8_MEMBER( io2_r );

	virtual void machine_reset();
	DECLARE_PALETTE_INIT(laser3k);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	UINT8 m_bank0val, m_bank1val, m_bank2val, m_bank3val;
	int m_flash;
	int m_speaker_state;
	int m_disp_page;
	int m_bg_color;

	void plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, UINT32 code, const UINT8 *textgfx_data, UINT32 textgfx_datalen);
};

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

static ADDRESS_MAP_START( laser3k_map, AS_PROGRAM, 8, laser3k_state )
	AM_RANGE(0x0000, 0x3fff) AM_DEVICE("bank0", address_map_bank_device, amap8)
	AM_RANGE(0x4000, 0x7fff) AM_DEVICE("bank1", address_map_bank_device, amap8)
	AM_RANGE(0x8000, 0xbfff) AM_DEVICE("bank2", address_map_bank_device, amap8)
	AM_RANGE(0xc000, 0xffff) AM_DEVICE("bank3", address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( banks_map, AS_PROGRAM, 8, laser3k_state )
	AM_RANGE(0x00000, 0x2ffff) AM_READWRITE(ram_r, ram_w)
	AM_RANGE(0x38000, 0x3bfff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x3c000, 0x3c0ff) AM_READWRITE(io_r, io_w)
	AM_RANGE(0x3c100, 0x3c1ff) AM_READ(io2_r)
	AM_RANGE(0x3c200, 0x3ffff) AM_ROM AM_REGION("maincpu", 0x4200)
ADDRESS_MAP_END

void laser3k_state::machine_reset()
{
	m_bank0val = 0;
	m_bank1val = 1;
	m_bank2val = 2;
	m_bank3val = 0xf;
	m_bank0->set_bank(m_bank0val);
	m_bank1->set_bank(m_bank1val);
	m_bank2->set_bank(m_bank2val);
	m_bank3->set_bank(m_bank3val);

	// reset the 6502 here with the banking set up so we get the right vector
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

	m_flash = 0;
	m_speaker_state = 0;
	m_disp_page = 0;
	m_bg_color = 0;

	UINT8 *rom = (UINT8 *)memregion("maincpu")->base();

	// patch out disk controller ID for now so it drops right into BASIC
	rom[0x4607] = 0;
}

READ8_MEMBER( laser3k_state::ram_r )
{
	return m_ram->read(offset);
}

WRITE8_MEMBER( laser3k_state::ram_w )
{
	m_ram->write(offset, data);
}

READ8_MEMBER( laser3k_state::io_r )
{
	switch (offset)
	{
		case 0x00:	// keyboard latch
			return 0x00;

		case 0x08:	// set border color to black
			break;

		case 0x10:	// keyboard strobe
			return 0x00;

		case 0x18:	// set bg color to black
			m_bg_color = 0;
			break;
		case 0x19:	// set bg color to red
			m_bg_color = 1;
			break;
		case 0x1a:	// set bg color to green
			m_bg_color = 12;
			break;
		case 0x1b:	// set bg color to yellow
			m_bg_color = 13;
			break;
		case 0x1c:	// set bg color to blue
			m_bg_color = 6;
			break;
		case 0x1d:	// set bg color to magenta
			m_bg_color = 3;
			break;
		case 0x1e:	// set bg color to cyan
			m_bg_color = 14;
			break;
		case 0x1f:	// set bg color to white
			m_bg_color = 15;
			break;

		case 0x28:	// "enable multi-color mode" - not sure what this means
			break;

		case 0x4c:	// low resolution (40 column)
			break;

		case 0x30:
			m_speaker_state ^= 1;
			m_speaker->level_w(m_speaker_state);
			break;

		case 0x51:	// text mode
			break;

		case 0x54:	// set page 1
			m_disp_page = 0;
			break;

		case 0x55:	// set page 2
			m_disp_page = 1;
			break;

		case 0x56:	// disable emulation (?)
			break;

		case 0x7c:
			return m_bank0val;
			
		case 0x7d:
			return m_bank1val;
			
		case 0x7e:
			return m_bank2val;
			
		case 0x7f:
			return m_bank3val;
			
		default:
			printf("io_r @ unknown %x\n", offset);
			break;
	}

	return 0xff;
}

WRITE8_MEMBER( laser3k_state::io_w )
{
	switch (offset)
	{
		case 0x10:	// clear keyboard latch
			break;

		case 0x30:	// speaker toggle sound
			m_speaker_state ^= 1;
			m_speaker->level_w(m_speaker_state);
			break;

		case 0x4c:	// low resolution
			break;

		case 0x68:	// SN76489 sound
			m_sn->write(space, 0, data);
			break;

		case 0x78:	// called "SYSTEM" in the boot ROM listing, but unsure what it does
			break;

		case 0x7c:	// bank 0
			m_bank0val = data;
			m_bank0->set_bank(m_bank0val);
			break;

		case 0x7d:  // bank 1
			m_bank1val = data;
			m_bank1->set_bank(m_bank1val);
			break;

		case 0x7e:	// bank 2
			m_bank2val = data;
			m_bank2->set_bank(m_bank2val);
			break;

		case 0x7f:	// bank 3
			m_bank3val = data;
			m_bank3->set_bank(m_bank3val);
			break;

		default:
			printf("io_w %02x @ unknown %x\n", data, offset);
			break;
	}
}

READ8_MEMBER( laser3k_state::io2_r )
{
	switch (offset)
	{
		case 0xc2:	// h-blank status
			return space.machine().first_screen()->hblank() ? 0x80 : 0x00;

		case 0xc3:	// v-blank status
			return space.machine().first_screen()->vblank() ? 0x80 : 0x00;

		case 0xc5:	// CPU 1/2 MHz status?
			return 0x00;
			
		default:
			printf("io2_r @ unknown %x\n", offset);
			break;
	}

	return 0xff;
}

void laser3k_state::plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, UINT32 code,
	const UINT8 *textgfx_data, UINT32 textgfx_datalen)
{
	int x, y, i;
	int fg = 15;
	int bg = m_bg_color;
	const UINT8 *chardata;
	UINT16 color;

	/* look up the character data */
	chardata = &textgfx_data[(code * 8) % textgfx_datalen];

	if (m_flash && (code >= 0x40) && (code <= 0x7f))
	{
		/* we're flashing; swap */
		i = fg;
		fg = bg;
		bg = i;
	}

	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 7; x++)
		{
			color = (chardata[y] & (1 << (6-x))) ? fg : bg;

			for (i = 0; i < xscale; i++)
			{
				bitmap.pix16(ypos + y, xpos + (x * xscale) + i) = color; 
			}
		}
	}
}

UINT32 laser3k_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int row, col;
	UINT32 start_address = (m_disp_page == 0) ? 0x400 : 0x800;
	UINT32 address;
	UINT8 *m_a2_videoram = m_ram->pointer();
	int beginrow = 0, endrow = 191;

	m_flash = ((machine().time() * 4).seconds & 1) ? 1 : 0;

	beginrow = MAX(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = MIN(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	for (row = beginrow; row <= endrow; row += 8)
	{
		for (col = 0; col < 40; col++)
		{
			/* calculate address */
			address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

			plot_text_character(bitmap, col * 14, row, 2, m_a2_videoram[address],
				memregion("gfx1")->base(), memregion("gfx1")->bytes());
		}
	}

	return 0;
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( laser3k )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)
INPUT_PORTS_END

// this is an apple II palette; it seems more likely the
// actual laser3000 has a digital RGB palette...
static const rgb_t laser3k_palette[] =
{
	rgb_t::black,
	rgb_t(0xE3, 0x1E, 0x60), /* Dark Red */
	rgb_t(0x60, 0x4E, 0xBD), /* Dark Blue */
	rgb_t(0xFF, 0x44, 0xFD), /* Purple */
	rgb_t(0x00, 0xA3, 0x60), /* Dark Green */
	rgb_t(0x9C, 0x9C, 0x9C), /* Dark Gray */
	rgb_t(0x14, 0xCF, 0xFD), /* Medium Blue */
	rgb_t(0xD0, 0xC3, 0xFF), /* Light Blue */
	rgb_t(0x60, 0x72, 0x03), /* Brown */
	rgb_t(0xFF, 0x6A, 0x3C), /* Orange */
	rgb_t(0x9C, 0x9C, 0x9C), /* Light Grey */
	rgb_t(0xFF, 0xA0, 0xD0), /* Pink */
	rgb_t(0x14, 0xF5, 0x3C), /* Light Green */
	rgb_t(0xD0, 0xDD, 0x8D), /* Yellow */
	rgb_t(0x72, 0xFF, 0xD0), /* Aquamarine */
	rgb_t(0xFF, 0xFF, 0xFF)  /* White */
};

/* Initialize the palette */
PALETTE_INIT_MEMBER(laser3k_state, laser3k)
{
	palette.set_pen_colors(0, laser3k_palette, ARRAY_LENGTH(laser3k_palette));
}

static MACHINE_CONFIG_START( laser3k, laser3k_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 1021800)
	MCFG_CPU_PROGRAM_MAP(laser3k_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(300*2, 192)
	MCFG_SCREEN_VISIBLE_AREA(0, (280*2)-1,0,192-1)
	MCFG_SCREEN_UPDATE_DRIVER(laser3k_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(laser3k_palette))
	MCFG_PALETTE_INIT_OWNER(laser3k_state, laser3k)

	/* memory banking */
	MCFG_DEVICE_ADD("bank0", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(banks_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x4000)
	MCFG_DEVICE_ADD("bank1", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(banks_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x4000)
	MCFG_DEVICE_ADD("bank2", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(banks_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x4000)
	MCFG_DEVICE_ADD("bank3", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(banks_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x4000)

	MCFG_RAM_ADD("mainram")
	MCFG_RAM_DEFAULT_SIZE("192K")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_ADD("sn76489", SN76489, 1020484)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

ROM_START(las3000)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "341-0036.chr", 0x0000, 0x0800, CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659))

	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "las3000.rom", 0x0000, 0x8000, CRC(9C7AEB09) SHA1(3302ADF41E258CF50210C19736948C8FA65E91DE))

	ROM_REGION(0x100, "fdc", 0)
	ROM_LOAD ( "l3kdisk.rom", 0x0000, 0x0100, CRC(2D4B1584) SHA1(989780B77E100598124DF7B72663E5A31A3339C0))
ROM_END

/*    YEAR  NAME      PARENT    COMPAT    MACHINE      INPUT     INIT      COMPANY          FULLNAME */
COMP( 1983, las3000,  0,        0,        laser3k,     laser3k, driver_device,  0,        "Video Technology",  "Laser 3000",    GAME_NOT_WORKING )
