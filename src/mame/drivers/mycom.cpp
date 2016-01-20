// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Robbbert
/******************************************************************************

    MYCOMZ-80A (c) 1981 Japan Electronics College

    preliminary driver by Angelo Salese
    additions by Robbbert

    THE KEYBOARD
    - Uses a 4MO1003A custom mcu.
    - Every combination is documented in the manual, including the unused ones.
    - All commands must be in uppercase - lowercase will produce errors.
    - There are 5 special keys which do crude editing functions. These are in
      the range 0x61 to 0x75. Kana characters occupy the range 0xa0 to 0xff.
      Graphics characters are found in 0x00 to 0x1f, and 0x80 to 0x9f.
    - Editing characters (hold down shift to get them):
        a - shiftlock (toggle). You can then enter any lowercase character.
        c - clear screen and home cursor
        d - insert
        f - vertical tab (cursor up) You can scroll backwards with this,
            and you can reuse old input lines.
        u - cursor right
    - There are switches on the right-hand side which are connected directly
      to one of the PIAs. The switches (not emulated):
        s2 - ?unknown
        s3 - ?unknown but must be high for the keyboard to function
        s4 - cassette motor on/off
        s5 - ?unknown
    - There is also switch s1 which it is not known what it connects to.
    - Please note: The Takeda 80-column monitor expects the enter key to be the
      line feed key. This is the numpad-enter in our emulation. Strangely, the
      standard monitor, and Basic, will also respond to this key.
    - The keyboard has a "English" key on the left, and a "Japan" key on the
      right. Pressing the appropriate key toggles the input language mode.
      Internally, this turns the Kana bit off/on. On our keyboard, the ALT key
      toggles between English and Kana.

    TODO/info:
    - Sound not working. The info makes its way to the audio chip but for
      some unknown reason, nothing is heard.
    - FDC, little info, guessing (143kb, single sided, 525sd)
    - Cassette doesn't load
    - Printer
    - Keyboard lookup table for Kana and Shifted Kana
    - Keyboard autorepeat
    - Need software

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/i8255.h"
#include "sound/sn76496.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "machine/msm5832.h"
#include "machine/wd_fdc.h"


class mycom_state : public driver_device
{
public:
	mycom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi0(*this, "ppi8255_0")
		, m_ppi1(*this, "ppi8255_1")
		, m_ppi2(*this, "ppi8255_2")
		, m_cass(*this, "cassette")
		, m_wave(*this, WAVE_TAG)
		, m_crtc(*this, "crtc")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_audio(*this, "sn1")
		, m_rtc(*this, "rtc"),
		m_palette(*this, "palette")
	{ }

	DECLARE_READ8_MEMBER(mycom_upper_r);
	DECLARE_WRITE8_MEMBER(mycom_upper_w);
	DECLARE_READ8_MEMBER(vram_data_r);
	DECLARE_WRITE8_MEMBER(vram_data_w);
	DECLARE_WRITE8_MEMBER(mycom_00_w);
	DECLARE_WRITE8_MEMBER(mycom_04_w);
	DECLARE_WRITE8_MEMBER(mycom_06_w);
	DECLARE_WRITE8_MEMBER(mycom_0a_w);
	DECLARE_READ8_MEMBER(mycom_05_r);
	DECLARE_READ8_MEMBER(mycom_06_r);
	DECLARE_READ8_MEMBER(mycom_08_r);
	DECLARE_DRIVER_INIT(mycom);
	TIMER_DEVICE_CALLBACK_MEMBER(mycom_kbd);
	DECLARE_WRITE8_MEMBER(mycom_rtc_w);
	MC6845_UPDATE_ROW(crtc_update_row);
	UINT8 *m_p_videoram;
	UINT8 *m_p_chargen;
	UINT8 m_0a;
private:
	UINT16 m_i_videoram;
	UINT8 m_keyb_press;
	UINT8 m_keyb_press_flag;
	UINT8 m_sn_we;
	UINT32 m_upper_sw;
	UINT8 *m_p_ram;
	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void video_start() override;
	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi0;
	required_device<i8255_device> m_ppi1;
	required_device<i8255_device> m_ppi2;
	required_device<cassette_image_device> m_cass;
	required_device<wave_device> m_wave;
	required_device<mc6845_device> m_crtc;
	required_device<fd1771_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<sn76489_device> m_audio;
	required_device<msm5832_device> m_rtc;
public:
	required_device<palette_device> m_palette;
};



void mycom_state::video_start()
{
	m_p_videoram = memregion("vram")->base();
	m_p_chargen = memregion("chargen")->base();
}

MC6845_UPDATE_ROW( mycom_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 chr,gfx=0,z;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	if (m_0a & 0x40)
	{
		for (x = 0; x < x_count; x++)                   // lores pixels
		{
			UINT8 dbit=1;
			if (x == cursor_x) dbit=0;
			mem = (ma + x) & 0x7ff;
			chr = m_p_videoram[mem];
			z = ra / 3;
			*p++ = palette[BIT( chr, z ) ? dbit: dbit^1];
			*p++ = palette[BIT( chr, z ) ? dbit: dbit^1];
			*p++ = palette[BIT( chr, z ) ? dbit: dbit^1];
			*p++ = palette[BIT( chr, z ) ? dbit: dbit^1];
			z += 4;
			*p++ = palette[BIT( chr, z ) ? dbit: dbit^1];
			*p++ = palette[BIT( chr, z ) ? dbit: dbit^1];
			*p++ = palette[BIT( chr, z ) ? dbit: dbit^1];
			*p++ = palette[BIT( chr, z ) ? dbit: dbit^1];
		}
	}
	else
	{
		for (x = 0; x < x_count; x++)                   // text
		{
			UINT8 inv=0;
			if (x == cursor_x) inv=0xff;
			mem = (ma + x) & 0x7ff;
			if (ra > 7)
				gfx = inv;  // some blank spacing lines
			else
			{
				chr = m_p_videoram[mem];
				gfx = m_p_chargen[(chr<<3) | ra] ^ inv;
			}

			/* Display a scanline of a character */
			*p++ = palette[BIT(gfx, 7)];
			*p++ = palette[BIT(gfx, 6)];
			*p++ = palette[BIT(gfx, 5)];
			*p++ = palette[BIT(gfx, 4)];
			*p++ = palette[BIT(gfx, 3)];
			*p++ = palette[BIT(gfx, 2)];
			*p++ = palette[BIT(gfx, 1)];
			*p++ = palette[BIT(gfx, 0)];
		}
	}
}

WRITE8_MEMBER( mycom_state::mycom_00_w )
{
	switch(data)
	{
		case 0x00: membank("boot")->set_entry(1); break;
		case 0x01: membank("boot")->set_entry(0); break;
		case 0x02: m_upper_sw = 0x10000; break;
		case 0x03: m_upper_sw = 0x0c000; break;
	}
}

READ8_MEMBER( mycom_state::mycom_upper_r )
{
	return m_p_ram[offset | m_upper_sw];
}

WRITE8_MEMBER( mycom_state::mycom_upper_w )
{
	m_p_ram[offset | 0xc000] = data;
}

READ8_MEMBER( mycom_state::vram_data_r )
{
	return m_p_videoram[m_i_videoram];
}

WRITE8_MEMBER( mycom_state::vram_data_w )
{
	m_p_videoram[m_i_videoram] = data;
}

static ADDRESS_MAP_START(mycom_map, AS_PROGRAM, 8, mycom_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_RAMBANK("boot")
	AM_RANGE(0x1000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xffff) AM_READWRITE(mycom_upper_r,mycom_upper_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mycom_io, AS_IO, 8, mycom_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(mycom_00_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(vram_data_r,vram_data_w)
	AM_RANGE(0x02, 0x02) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x03, 0x03) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("ppi8255_2", i8255_device, read, write)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("fdc", fd1771_t, read, write)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mycom )
	PORT_START("X0")
	PORT_BIT(0x001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CHAR(27)
	PORT_BIT(0x100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("X1")
	PORT_BIT(0x002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7 '") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("X2")
	PORT_BIT(0x002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q') PORT_CHAR(17)
	PORT_BIT(0x004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w') PORT_CHAR(23)
	PORT_BIT(0x008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e') PORT_CHAR(5)
	PORT_BIT(0x010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r') PORT_CHAR(18)
	PORT_BIT(0x020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t') PORT_CHAR(20)
	PORT_BIT(0x040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y') PORT_CHAR(25)
	PORT_BIT(0x080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u') PORT_CHAR(21)
	PORT_BIT(0x100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i') PORT_CHAR(9)
	PORT_BIT(0x200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o') PORT_CHAR(15)

	PORT_START("X3")
	PORT_BIT(0x002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a') PORT_CHAR(1)
	PORT_BIT(0x004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s') PORT_CHAR(19)
	PORT_BIT(0x008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d') PORT_CHAR(4)
	PORT_BIT(0x010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f') PORT_CHAR(6)
	PORT_BIT(0x020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g') PORT_CHAR(7)
	PORT_BIT(0x040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h') PORT_CHAR(8)
	PORT_BIT(0x080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j') PORT_CHAR(10)
	PORT_BIT(0x100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k') PORT_CHAR(11)
	PORT_BIT(0x200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l') PORT_CHAR(12)

	PORT_START("X4")
	PORT_BIT(0x001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z') PORT_CHAR(26)
	PORT_BIT(0x004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x') PORT_CHAR(24)
	PORT_BIT(0x008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c') PORT_CHAR(3)
	PORT_BIT(0x010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v') PORT_CHAR(22)
	PORT_BIT(0x020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b') PORT_CHAR(2)
	PORT_BIT(0x040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n') PORT_CHAR(14)
	PORT_BIT(0x080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m') PORT_CHAR(13)
	PORT_BIT(0x100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("< ,") PORT_CODE(KEYCODE_COMMA) PORT_CHAR('<') PORT_CHAR(',')
	PORT_BIT(0x200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("> .") PORT_CODE(KEYCODE_STOP) PORT_CHAR('>') PORT_CHAR('.')

	PORT_START("X5")
	PORT_BIT(0x020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(u)") PORT_CHAR('u')
	PORT_BIT(0x200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(c)") PORT_CHAR('c')

	PORT_START("X6")
	PORT_BIT(0x001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~') PORT_CHAR(30)
	PORT_BIT(0x008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\\ _") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('_') PORT_CHAR(28)
	PORT_BIT(0x020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(d)") PORT_CHAR('d')
	PORT_BIT(0x200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(f)") PORT_CHAR('f')

	PORT_START("X7")
	PORT_BIT(0x001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p') PORT_CHAR(16)
	PORT_BIT(0x002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('`') PORT_CHAR(0)
	PORT_BIT(0x004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') PORT_CHAR(27)
	PORT_BIT(0x008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("LineFeed") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(10)
	PORT_BIT(0x010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(a)") PORT_CHAR('a')

	PORT_START("X8")
	PORT_BIT(0x001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(29)
	PORT_BIT(0x020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("XX")
	PORT_BIT(0x001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("KANA") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) PORT_TOGGLE
INPUT_PORTS_END


/* F4 Character Displayer */
static const gfx_layout mycom_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( mycom )
	GFXDECODE_ENTRY( "chargen", 0x0000, mycom_charlayout, 0, 1 )
GFXDECODE_END

WRITE8_MEMBER( mycom_state::mycom_04_w )
{
	m_i_videoram = (m_i_videoram & 0x700) | data;

	m_sn_we = data;
}

WRITE8_MEMBER( mycom_state::mycom_06_w )
{
	m_i_videoram = (m_i_videoram & 0x0ff) | ((data & 0x007) << 8);
}

READ8_MEMBER( mycom_state::mycom_08_r )
{
	/*
	x--- ---- display flag
	---- --x- keyboard shift
	---- ---x keyboard strobe
	*/
	UINT8 data = 0;

	data = m_keyb_press_flag; //~m_keyb_press_flag & 1;

	if ((m_cass)->input() > 0.03) // not working
		data+=4;

	return data;
}

READ8_MEMBER( mycom_state::mycom_06_r )
{
	/*
	x--- ---- keyboard s5
	-x-- ---- keyboard s4 (motor on/off)
	--x- ---- keyboard s3 (must be high)
	---x ---- keyboard s2
	*/
	return 0xff;
}

READ8_MEMBER( mycom_state::mycom_05_r )
{
	return m_keyb_press;
}

WRITE8_MEMBER( mycom_state::mycom_0a_w )
{
	/*
	x--- ---- width 80/40 (0 = 80, 1 = 40)
	-x-- ---- video mode (0= tile, 1 = bitmap)
	--x- ---- PSG Chip Select bit
	---x ---- PSG Write Enable bit
	---- x--- cmt remote (defaults to on)
	---- -x-- cmt output
	---- --x- printer reset
	---- ---x printer strobe
	*/

	if ( (BIT(m_0a, 3)) != (BIT(data, 3)) )
		m_cass->change_state(
		BIT(data,3) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	if (BIT(data, 3)) // motor on
		m_cass->output( BIT(data, 2) ? -1.0 : +1.0);

	if ( (BIT(data, 7)) != (BIT(m_0a, 7)) )
		m_crtc->set_clock(BIT(data, 7) ? 1008000 : 2016000);

	m_0a = data;

	// if WE & CE are low, pass sound command to audio chip
	if ((data & 0x30)==0)
		m_audio->write(space, 0, m_sn_we);
}

WRITE8_MEMBER(mycom_state::mycom_rtc_w)
{
	m_rtc->address_w(data & 0x0f);

	m_rtc->hold_w(BIT(data, 4));
	m_rtc->read_w(BIT(data, 5));
	m_rtc->write_w(BIT(data, 6));
	m_rtc->cs_w(BIT(data, 7));
}

static const UINT8 mycom_keyval[] = { 0,
0x1b,0x1b,0x7c,0x7c,0x18,0x18,0x0f,0x0f,0x09,0x09,0x1c,0x1c,0x30,0x00,0x50,0x70,0x3b,0x2b,
0x00,0x00,0x31,0x21,0x51,0x71,0x41,0x61,0x5a,0x7a,0x17,0x17,0x2d,0x3d,0x40,0x60,0x3a,0x2a,
0x0b,0x0b,0x32,0x22,0x57,0x77,0x53,0x73,0x58,0x78,0x03,0x03,0x5e,0x7e,0x5b,0x7b,0x5d,0x7d,
0x1f,0x1f,0x33,0x23,0x45,0x65,0x44,0x64,0x43,0x63,0x0c,0x0c,0x5c,0x7c,0x0a,0x0a,0x00,0x00,
0x01,0x01,0x34,0x24,0x52,0x72,0x46,0x66,0x56,0x76,0x04,0x04,0x7f,0x7f,0x08,0x08,0x0e,0x0e,
0x02,0x02,0x35,0x25,0x54,0x74,0x47,0x67,0x42,0x62,0x75,0x75,0x64,0x64,0x00,0x00,0x20,0x20,
0x1e,0x1e,0x36,0x26,0x59,0x79,0x48,0x68,0x4e,0x6e,0x37,0x37,0x34,0x34,0x31,0x31,0x30,0x30,
0x1d,0x1d,0x37,0x27,0x55,0x75,0x4a,0x6a,0x4d,0x6d,0x38,0x38,0x35,0x35,0x32,0x32,0x11,0x11,
0x0d,0x0d,0x38,0x28,0x49,0x69,0x4b,0x6b,0x2c,0x3c,0x39,0x39,0x36,0x36,0x33,0x33,0x12,0x12,
0x07,0x07,0x39,0x29,0x4f,0x6f,0x4c,0x6c,0x2e,0x3e,0x63,0x63,0x66,0x66,0x61,0x61,0x2f,0x3f };

TIMER_DEVICE_CALLBACK_MEMBER(mycom_state::mycom_kbd)
{
	UINT8 x, y, scancode = 0;
	UINT16 pressed[9];
	char kbdrow[3];
	UINT8 modifiers = ioport("XX")->read();
	UINT8 shift_pressed = (modifiers & 2) >> 1;
	m_keyb_press_flag = 0;

	/* see what is pressed */
	for (x = 0; x < 9; x++)
	{
		sprintf(kbdrow,"X%d",x);
		pressed[x] = (ioport(kbdrow)->read());
	}

	/* find what has changed */
	for (x = 0; x < 9; x++)
	{
		if (pressed[x])
		{
			/* get scankey value */
			for (y = 0; y < 10; y++)
			{
				if (BIT(pressed[x], y))
				{
					scancode = ((x + y * 9) << 1) + shift_pressed + 1;
					m_keyb_press_flag = 1;
					m_keyb_press = mycom_keyval[scancode];
				}
			}
		}
	}

	if (m_keyb_press_flag)
	{
		if (modifiers & 1) m_keyb_press &= 0xbf;
		if (modifiers & 4) m_keyb_press |= 0x80;
	}
}



static SLOT_INTERFACE_START( mycom_floppies )
	SLOT_INTERFACE( "525sd", FLOPPY_525_SD )
SLOT_INTERFACE_END

void mycom_state::machine_start()
{
	m_p_ram = memregion("maincpu")->base();
}

void mycom_state::machine_reset()
{
	membank("boot")->set_entry(1);
	m_upper_sw = 0x10000;
	m_0a = 0;
}

DRIVER_INIT_MEMBER(mycom_state,mycom)
{
	UINT8 *RAM = memregion("maincpu")->base();
	membank("boot")->configure_entries(0, 2, &RAM[0x0000], 0x10000);
}

static MACHINE_CONFIG_START( mycom, mycom_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_10MHz / 4)
	MCFG_CPU_PROGRAM_MAP(mycom_map)
	MCFG_CPU_IO_MAP(mycom_io)

	MCFG_DEVICE_ADD("ppi8255_0", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(mycom_state, mycom_04_w))
	MCFG_I8255_IN_PORTB_CB(READ8(mycom_state, mycom_05_r))
	MCFG_I8255_IN_PORTC_CB(READ8(mycom_state, mycom_06_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mycom_state, mycom_06_w))

	MCFG_DEVICE_ADD("ppi8255_1", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(mycom_state, mycom_08_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mycom_state, mycom_0a_w))

	MCFG_DEVICE_ADD("ppi8255_2", I8255, 0)
	MCFG_I8255_IN_PORTB_CB(DEVREAD8("rtc", msm5832_device, data_r))
	MCFG_I8255_OUT_PORTB_CB(DEVWRITE8("rtc", msm5832_device, data_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mycom_state, mycom_rtc_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 192-1)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mycom)

	/* Manual states clock is 1.008mhz for 40 cols, and 2.016 mhz for 80 cols.
	The CRTC is a HD46505S - same as a 6845. The start registers need to be readable. */
	MCFG_MC6845_ADD("crtc", MC6845, "screen", 1008000)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(mycom_state, crtc_update_row)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("sn1", SN76489, XTAL_10MHz / 4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.50)

	/* Devices */
	MCFG_MSM5832_ADD("rtc", XTAL_32_768kHz)
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_FD1771_ADD("fdc", XTAL_16MHz / 16)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", mycom_floppies, "525sd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", mycom_floppies, "525sd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard_timer", mycom_state, mycom_kbd, attotime::from_hz(20))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mycom )
	ROM_REGION( 0x14000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("mycom")
	ROM_SYSTEM_BIOS(0, "mycom", "40 column")
	ROMX_LOAD( "bios0.rom", 0x10000, 0x3000, CRC(e6f50355) SHA1(5d3acea360c0a8ab547db03a43e1bae5125f9c2a), ROM_BIOS(1))
	ROMX_LOAD( "basic0.rom",0x13000, 0x1000, CRC(3b077465) SHA1(777427182627f371542c5e0521ed3ca1466a90e1), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "Takeda", "80 column")
	ROMX_LOAD( "bios1.rom", 0x10000, 0x3000, CRC(c51d7fcb) SHA1(31d39db43b77cca4d49ff9814d531e056924e716), ROM_BIOS(2))
	ROMX_LOAD( "basic1.rom",0x13000, 0x1000, CRC(30a573f1) SHA1(e3fe2e73644e831b52e2789dc7c181989cc30b82), ROM_BIOS(2))
	/* Takeda bios has no cursor. Use the next lines to turn on cursor, but you must comment out when done. */
	//ROM_FILL( 0x11eb6, 1, 0x47 )
	//ROM_FILL( 0x11eb7, 1, 0x07 )

	ROM_REGION( 0x0800, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x0000, 0x0800, CRC(4039bb6f) SHA1(086ad303bf4bcf983fd6472577acbf744875fea8) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT        COMPANY                   FULLNAME       FLAGS */
COMP( 1981, mycom,  0,      0,       mycom,     mycom, mycom_state,   mycom, "Japan Electronics College", "MYCOMZ-80A", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
